<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import Stepper from '~icons/tabler/hand-move';
	import Settings from '~icons/tabler/adjustments';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Reload from '~icons/tabler/reload';
	import Stop from '~icons/tabler/hand-stop';
	import Spinner from '$lib/components/Spinner.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';

	type StepperControl = {
		isEnabled: boolean;
		direction: boolean;
		speed: number;
		move: number;
		acceleration: number;
	};

	let stepperControl: StepperControl = { isEnabled: false, direction: false, speed: 128, move: 0, acceleration: 30 };

	const ws_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : '';

	const stepperControlSocket = new WebSocket('ws://' + $page.url.host + '/ws/stepperControl' + ws_token);

	stepperControlSocket.onopen = (event) => {
		stepperControlSocket.send('Hello');
	};

	stepperControlSocket.addEventListener('close', (event) => {
		const closeCode = event.code;
		const closeReason = event.reason;
		console.log('WebSocket closed with code:', closeCode);
		console.log('Close reason:', closeReason);
		notifications.error('Websocket disconnected', 5000);
	});

	stepperControlSocket.onmessage = (event) => {
		const message = JSON.parse(event.data);
		if (message.type != 'id') {
			stepperControl = message;
		}
	};

	onDestroy(() => stepperControlSocket.close());

	type StepperSettings = {
		enableOnStart: boolean;
		invertDirection: boolean;
		maxSpeed: number;
		maxAcceleration: number;
		current: number;
	};

	let stepperSettings: StepperSettings;

	let formField: any;

	async function getStepperSettings() {
		try {
			const response = await fetch('/rest/stepperSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			stepperSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postStepperSettings() {
		try {
			const response = await fetch('/rest/stepperSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(stepperSettings)
			});
			if (response.status == 200) {
				notifications.success('Stepper settings updated.', 3000);
				stepperSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

</script>


<style>
	.grid-form {
	  grid-template-columns: [labels] auto [controls] 1fr;
	}
	/* .grid-form {
	  display: grid;
	  grid-template-columns: [labels] auto [controls] 1fr;
	  grid-auto-flow: row;
	}
	.grid-form label  {
	  grid-column: labels;
	  grid-row: auto;
	}
	.grid-form input {
	  grid-column: controls;
	  grid-row: auto;
	} */
</style>

<SettingsCard collapsible={true} open={true}>
	<Stepper slot="icon" class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Stepper Control</span>
	<div class="w-full">
		<div class="w-full grid grid-flow-row grid-form items-center">
			<label class="label cursor-pointer" for="enable">
				<span class="">Enable</span>
			</label>
			<input
				type="checkbox"
				class="toggle toggle-primary"
				id="enable"
				bind:checked={stepperControl.isEnabled}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="direction">
				<span class="">Direction</span>
			</label>
			<input
				type="checkbox"
				class="toggle toggle-primary"
				id="direction"
				bind:checked={stepperControl.direction}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="speed">
				<span class="mr-4">Speed </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="1" 
				step="0.01"
				class="range range-primary"
				id="speed"
				bind:value={stepperControl.speed}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify({speed:stepperControl.speed}));
					console.log(JSON.stringify({speed:stepperControl.speed}));
				}}
			/>
			<label class="label cursor-pointer" for="move">
				<span class="mr-4">Move </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="1" 
				step="0.01"
				class="range range-primary"
				id="move"
				bind:value={stepperControl.move}
				on:input={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="acceleration">
				<span class="mr-4">Acceleration </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="1" 
				step="0.01"
				class="range range-primary"
				id="acceleration"
				bind:value={stepperControl.acceleration}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<!-- <label class="label cursor-pointer" for="current">
				<span class="mr-4">Current </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="4000" 
				class="range range-primary"
				id="current"
				bind:value={stepperControl.current}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/> -->
		</div>
		<div class="flex flex-row flex-wrap justify-between gap-x-2">
			<div class="flex-grow"></div>
			<div>
				<div>
					<button class="btn btn-primary inline-flex items-center" on:click={() => {stepperControl.speed=0; stepperControlSocket.send(JSON.stringify(stepperControl));}}
						><Stop class="mr-2 h-5 w-5" /><span>Stop</span></button
					>
				</div>
			</div>
		</div>
	</div>
	<!-- {#if !$page.data.features.security || $user.admin}
	<Collapsible open={false} class="shadow-lg">
		<span slot="title">Driver Settings</span>

		<form on:submit|preventDefault={""} novalidate>
		</form>
	</Collapsible>
{/if} -->
</SettingsCard>

<SettingsCard collapsible={true} open={false}>
	<Settings slot="icon" class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Stepper Settings</span>
	<div class="w-full overflow-x-auto">
		{#await getStepperSettings()}
			<Spinner />
		{:then nothing}
			<form
				on:submit|preventDefault={postStepperSettings}
				novalidate
				bind:this={formField}
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="w-full">
					<div class="w-full grid grid-flow-row grid-form items-center">
						<label class="label cursor-pointer" for="enableonstart">
							<span class="">Enable on Start</span>
						</label>
						<input
							type="checkbox"
							class="toggle toggle-primary"
							id="enableonstart"
							bind:checked={stepperSettings.enableOnStart}
						/>
						<label class="label cursor-pointer" for="invdirection">
							<span class="">Invert Direction</span>
						</label>
						<input
							type="checkbox"
							class="toggle toggle-primary"
							id="invdirection"
							bind:checked={stepperSettings.invertDirection}
						/>
						<label class="label cursor-pointer" for="maxSpeed">
							<span class="mr-4">Max Speed </span>
						</label>
						<input 
							type="range"
							min="0" 
							max="400" 
							class="range range-primary"
							id="maxSpeed"
							bind:value={stepperSettings.maxSpeed}
						/>
						<label class="label cursor-pointer" for="maxAcceleration">
							<span class="mr-4">Max Acceleration </span>
						</label>
						<input 
							type="range"
							min="0" 
							max="200" 
							class="range range-primary"
							id="maxAcceleration"
							bind:value={stepperSettings.maxAcceleration}
						/>
						<label class="label cursor-pointer" for="current">
							<span class="mr-4">Driver Current </span>
						</label>
						<input 
							type="range"
							min="0" 
							max="4000" 
							class="range range-primary"
							id="current"
							bind:value={stepperSettings.current}
						/>
					</div>
				</div>
				<!-- <div class="alert alert-info my-2 shadow-lg">
					<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
					<span
						>The LED is controllable via MQTT with the demo project designed to work with Home
						Assistant's auto discovery feature.</span
					>
				</div>
				<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4">
					<div>
						<label class="label" for="uid">
							<span class="label-text text-md">Unique ID</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.uid
								? 'border-error border-2'
								: ''}"
							bind:value={stepperSettings.unique_id}
							id="uid"
							min="3"
							max="32"
							required
						/>
						<label class="label" for="uid">
							<span class="label-text-alt text-error {formErrors.uid ? '' : 'hidden'}"
								>Unique ID must be between 3 and 32 characters long</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="name">
							<span class="label-text text-md">Name</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.name
								? 'border-error border-2'
								: ''}"
							bind:value={stepperSettings.name}
							id="name"
							min="3"
							max="32"
							required
						/>
						<label class="label" for="name">
							<span class="label-text-alt text-error {formErrors.name ? '' : 'hidden'}"
								>Name must be between 3 and 32 characters long</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="path">
							<span class="label-text text-md">MQTT Path</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.path
								? 'border-error border-2'
								: ''}"
							bind:value={stepperSettings.mqtt_path}
							id="path"
							min="0"
							max="64"
							required
						/>
						<label class="label" for="path">
							<span class="label-text-alt text-error {formErrors.path ? '' : 'hidden'}"
								>MQTT path is limited to 64 characters</span
							>
						</label>
					</div>
				</div> -->
				<div class="divider mb-2 mt-0" />
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		{/await}
	</div>
</SettingsCard>
